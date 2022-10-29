// Copyright 2021 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/** @fileoverview Definitions for chrome.autofillPrivate API */
// TODO(crbug.com/1203307): Auto-generate this file.

import {ChromeEvent} from './chrome_event.js';

declare global {
  export namespace chrome {
    export namespace autofillPrivate {

      export enum AddressField {
        HONORIFIC = 'HONORIFIC',
        FULL_NAME = 'FULL_NAME',
        COMPANY_NAME = 'COMPANY_NAME',
        ADDRESS_LINES = 'ADDRESS_LINES',
        ADDRESS_LEVEL_1 = 'ADDRESS_LEVEL_1',
        ADDRESS_LEVEL_2 = 'ADDRESS_LEVEL_2',
        ADDRESS_LEVEL_3 = 'ADDRESS_LEVEL_3',
        POSTAL_CODE = 'POSTAL_CODE',
        SORTING_CODE = 'SORTING_CODE',
        COUNTRY_CODE = 'COUNTRY_CODE',
      }

      export interface AutofillMetadata {
        summaryLabel: string;
        summarySublabel?: string;
        isLocal?: boolean;
        isCached?: boolean;
        isMigratable?: boolean;
        isVirtualCardEnrollmentEligible?: boolean;
        isVirtualCardEnrolled?: boolean;
      }

      export interface AddressEntry {
        guid?: string;
        fullNames?: string[];
        honorific?: string;
        companyName?: string;
        addressLines?: string;
        addressLevel1?: string;
        addressLevel2?: string;
        addressLevel3?: string;
        postalCode?: string;
        sortingCode?: string;
        countryCode?: string;
        phoneNumbers?: string[];
        emailAddresses?: string[];
        languageCode?: string;
        metadata?: AutofillMetadata;
      }

      export interface CountryEntry {
        name?: string;
        countryCode?: string;
      }

      export interface AddressComponent {
        field: AddressField;
        fieldName: string;
        isLongField: boolean;
        placeholder?: string;
      }

      export interface AddressComponentRow {
        row: AddressComponent[];
      }

      export interface AddressComponents {
        components: AddressComponentRow[];
        languageCode: string;
      }

      export interface CreditCardEntry {
        guid?: string;
        name?: string;
        cardNumber?: string;
        expirationMonth?: string;
        expirationYear?: string;
        nickname?: string;
        network?: string;
        imageSrc?: string;
        metadata?: AutofillMetadata;
      }

      export interface ValidatePhoneParams {
        phoneNumbers: string[];
        indexOfNewNumber: number;
        countryCode: string;
      }

      export function saveAddress(address: AddressEntry): void;
      export function getCountryList(
          callback: (entries: CountryEntry[]) => void): void;
      export function getAddressComponents(
          countryCode: string,
          callback: (components: AddressComponents) => void): void;
      export function getAddressList(
          callback: (entries: AddressEntry[]) => void): void;
      export function saveCreditCard(card: CreditCardEntry): void;
      export function removeEntry(guid: string): void;
      export function validatePhoneNumbers(
          params: ValidatePhoneParams,
          callback: (numbers: string[]) => void): void;
      export function getCreditCardList(
          callback: (entries: CreditCardEntry[]) => void): void;
      export function maskCreditCard(guid: string): void;
      export function migrateCreditCards(): void;
      export function logServerCardLinkClicked(): void;
      export function setCreditCardFIDOAuthEnabledState(enabled: boolean): void;
      export function getUpiIdList(callback: (items: string[]) => void): void;
      export function addVirtualCard(cardId: string): void;
      export function removeVirtualCard(cardId: string): void;

      export const onPersonalDataChanged: ChromeEvent<
          (addresses: AddressEntry[], creditCards: CreditCardEntry[]) => void>;
    }
  }
}
